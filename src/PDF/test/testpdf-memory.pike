
int main() {
	object pdf=PDF.pdf();
	pdf->begin_page(300.00,300.00);        
	pdf->add_bookmark("Proba oldal",0,1);
	int font=pdf->findfont("Helvetica-Bold","host",0);
	pdf->setfont(font,12.0);
	pdf->show("asdasjdkalsjdkasjdasdgashda jhdgahsdg as");
	pdf->end_page();
	pdf->close();
	write("%s",pdf->generate());
}
